#include "ThreadCatalogModel.h"
#include <AK/JsonArray.h>
#include <AK/JsonObject.h>
#include <AK/JsonValue.h>
#include <LibCore/CHttpRequest.h>
#include <LibCore/CNetworkJob.h>
#include <LibCore/CNetworkResponse.h>
#include <stdio.h>

ThreadCatalogModel::ThreadCatalogModel()
{
    update();
}

ThreadCatalogModel::~ThreadCatalogModel()
{
}

void ThreadCatalogModel::update()
{
    CHttpRequest request;
    request.set_hostname("a.4cdn.org");
    request.set_path("/g/catalog.json");

    auto* job = request.schedule();

    job->on_finish = [job, this](bool success) {
        auto* response = job->response();
        dbg() << "job finished! success=" << success << ", response=" << response;
        dbg() << "payload size: " << response->payload().size();

        auto json = JsonValue::from_string(response->payload());

        if (json.is_array()) {
            JsonArray new_catalog;

            for (auto& page : json.as_array().values()) {
                if (!page.is_object())
                    continue;
                auto threads_value = page.as_object().get("threads");
                if (!threads_value.is_array())
                    continue;
                for (auto& thread : threads_value.as_array().values()) {
                    new_catalog.append(thread);
                }
            }

            m_catalog = move(new_catalog);
        }

        did_update();
    };
}

int ThreadCatalogModel::row_count(const GModelIndex&) const
{
    return m_catalog.size();
}

String ThreadCatalogModel::column_name(int column) const
{
    switch (column) {
    case Column::ThreadNumber:
        return "#";
    case Column::Text:
        return "Text";
    case Column::ReplyCount:
        return "Replies";
    case Column::ImageCount:
        return "Images";
    default:
        ASSERT_NOT_REACHED();
    }
}

GModel::ColumnMetadata ThreadCatalogModel::column_metadata(int column) const
{
    switch (column) {
    case Column::ThreadNumber:
        return { 70, TextAlignment::CenterRight };
    case Column::Text:
        return { 200, TextAlignment::CenterLeft };
    case Column::ReplyCount:
        return { 45, TextAlignment::CenterRight };
    case Column::ImageCount:
        return { 40, TextAlignment::CenterRight };
    default:
        ASSERT_NOT_REACHED();
    }
}

GVariant ThreadCatalogModel::data(const GModelIndex& index, Role role) const
{
    auto& thread = m_catalog.at(index.row()).as_object();
    if (role == Role::Display) {
        switch (index.column()) {
        case Column::ThreadNumber:
            return thread.get("no").to_u32();
        case Column::Text:
            return thread.get("com").to_string();
        case Column::ReplyCount:
            return thread.get("replies").to_u32();
        case Column::ImageCount:
            return thread.get("images").to_u32();
        default:
            ASSERT_NOT_REACHED();
        }
    }
    return {};
}
